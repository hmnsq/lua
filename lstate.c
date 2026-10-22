/*
** $Id: lstate.c $
** Global State
** See Copyright Notice in lua.h
*/

#define lstate_c
#define LUA_CORE

#अन्तर्भाव "lprefix.h"


#अन्तर्भाव <stddef.h>
#अन्तर्भाव <string.h>

#अन्तर्भाव "lua.h"

#अन्तर्भाव "lapi.h"
#अन्तर्भाव "ldebug.h"
#अन्तर्भाव "ldo.h"
#अन्तर्भाव "lfunc.h"
#अन्तर्भाव "lgc.h"
#अन्तर्भाव "llex.h"
#अन्तर्भाव "lmem.h"
#अन्तर्भाव "lstate.h"
#अन्तर्भाव "lstring.h"
#अन्तर्भाव "ltable.h"
#अन्तर्भाव "ltm.h"



#define fromstate(L)	(cast(LX *, cast(lu_byte *, (L)) - दुरत्व(LX, l)))


/*
** these macros allow user-specific actions when a thread is
** created/deleted
*/
#जर !defined(luai_userstateopen)
#define luai_userstateopen(L)		((शून्य)L)
#endif

#जर !defined(luai_userstateclose)
#define luai_userstateclose(L)		((शून्य)L)
#endif

#जर !defined(luai_userstatethread)
#define luai_userstatethread(L,L1)	((शून्य)L)
#endif

#जर !defined(luai_userstatefree)
#define luai_userstatefree(L,L1)	((शून्य)L)
#endif


/*
** set GCdebt to a new value keeping the real number of allocated
** objects (GCtotalobjs - GCdebt) invariant and avoiding overflows in
** 'GCtotalobjs'.
*/
शून्य luaE_setdebt (global_State *g, l_mem debt) {
  l_mem tb = gettotalbytes(g);
  lua_assert(tb > 0);
  जर (debt > MAX_LMEM - tb)
    debt = MAX_LMEM - tb;  /* will make GCtotalbytes == MAX_LMEM */
  g->GCtotalbytes = tb + debt;
  g->GCdebt = debt;
}


CallInfo *luaE_extendCI (lua_State *L, पूर्णांक err) {
  CallInfo *ci;
  ci = luaM_reallocvector(L, रिक्त, 0, 1, CallInfo);
  जर (l_unlikely(ci == रिक्त)) {  /* allocation failed? */
    जर (err)
      luaM_error(L);  /* raise the error */
    परत रिक्त;  /* else only report it */
  }
  ci->next = L->ci->next;
  ci->previous = L->ci;
  L->ci->next = ci;
  जर (ci->next)
    ci->next->previous = ci;
  ci->u.l.trap = 0;
  L->nci++;
  परत ci;
}


/*
** free all CallInfo structures not in use by a thread
*/
स्थिर शून्य freeCI (lua_State *L) {
  CallInfo *ci = L->ci;
  CallInfo *next = ci->next;
  ci->next = रिक्त;
  पर्यंत ((ci = next) != रिक्त) {
    next = ci->next;
    luaM_free(L, ci);
    L->nci--;
  }
}


/*
** free half of the CallInfo structures not in use by a thread,
** keeping the first one.
*/
शून्य luaE_shrinkCI (lua_State *L) {
  CallInfo *ci = L->ci->next;  /* first free CallInfo */
  CallInfo *next;
  जर (ci == रिक्त)
    परत;  /* no extra elements */
  पर्यंत ((next = ci->next) != रिक्त) {  /* two extra elements? */
    CallInfo *next2 = next->next;  /* next's next */
    ci->next = next2;  /* remove next from the list */
    L->nci--;
    luaM_free(L, next);  /* free next */
    जर (next2 == रिक्त)
      थांबा;  /* no more elements */
    अन्यथा {
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
शून्य luaE_checkcstack (lua_State *L) {
  जर (getCcalls(L) == LUAI_MAXCCALLS)
    luaG_runerror(L, "C stack overflow");
  अन्यथा जर (getCcalls(L) >= (LUAI_MAXCCALLS / 10 * 11))
    luaD_errerr(L);  /* error while handling stack error */
}


LUAI_FUNC शून्य luaE_incCstack (lua_State *L) {
  L->nCcalls++;
  जर (l_unlikely(getCcalls(L) >= LUAI_MAXCCALLS))
    luaE_checkcstack(L);
}


स्थिर शून्य resetCI (lua_State *L) {
  CallInfo *ci = L->ci = &L->base_ci;
  ci->func.p = L->चळत.p;
  setnilvalue2s(ci->func.p);  /* 'function' entry for basic 'ci' */
  ci->शीर्ष.p = ci->func.p + 1 + LUA_MINSTACK;  /* +1 for 'function' entry */
  ci->u.c.k = रिक्त;
  ci->callstatus = CIST_C;
  L->status = LUA_OK;
  L->errfunc = 0;  /* stack unwind can "throw away" the error function */
}


स्थिर शून्य चळत_init (lua_State *L1, lua_State *L) {
  पूर्णांक i;
  /* initialize stack array */
  L1->चळत.p = luaM_newvector(L, BASIC_चळत_SIZE + EXTRA_चळत, चळत_मूल्य);
  L1->tbclist.p = L1->चळत.p;
  साठी (i = 0; i < BASIC_चळत_SIZE + EXTRA_चळत; i++)
    setnilvalue2s(L1->चळत.p + i);  /* erase new stack */
  L1->चळत_last.p = L1->चळत.p + BASIC_चळत_SIZE;
  /* initialize first ci */
  resetCI(L1);
  L1->शीर्ष.p = L1->चळत.p + 1;  /* +1 for 'function' entry */
}


स्थिर शून्य freestack (lua_State *L) {
  जर (L->चळत.p == रिक्त)
    परत;  /* stack not completely built yet */
  L->ci = &L->base_ci;  /* free the entire 'ci' list */
  freeCI(L);
  lua_assert(L->nci == 0);
  /* free stack */
  luaM_freearray(L, L->चळत.p, cast_sizet(stacksize(L) + EXTRA_चळत));
}


/*
** Create registry table and its predefined values
*/
स्थिर शून्य init_registry (lua_State *L, global_State *g) {
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
स्थिर शून्य f_luaopen (lua_State *L, शून्य *ud) {
  global_State *g = G(L);
  UNUSED(ud);
  चळत_init(L, L);  /* init stack */
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
स्थिर शून्य preinit_thread (lua_State *L, global_State *g) {
  G(L) = g;
  L->चळत.p = रिक्त;
  L->ci = रिक्त;
  L->nci = 0;
  L->twups = L;  /* thread has no upvalues */
  L->nCcalls = 0;
  L->errorJmp = रिक्त;
  L->hook = रिक्त;
  L->hookmask = 0;
  L->basehookcount = 0;
  L->allowhook = 1;
  resethookcount(L);
  L->openupval = रिक्त;
  L->status = LUA_OK;
  L->errfunc = 0;
  L->oldpc = 0;
  L->base_ci.previous = L->base_ci.next = रिक्त;
}


lu_mem luaE_threadsize (lua_State *L) {
  lu_mem sz = cast(lu_mem, आकार(LX))
            + cast_uint(L->nci) * आकार(CallInfo);
  जर (L->चळत.p != रिक्त)
    sz += cast_uint(stacksize(L) + EXTRA_चळत) * आकार(चळत_मूल्य);
  परत sz;
}


स्थिर शून्य close_state (lua_State *L) {
  global_State *g = G(L);
  जर (!completestate(g))  /* closing a partially built state? */
    luaC_freeallobjects(L);  /* just collect its objects */
  अन्यथा {  /* closing a fully built state */
    resetCI(L);
    luaD_closeprotected(L, 1, LUA_OK);  /* close all upvalues */
    L->शीर्ष.p = L->चळत.p + 1;  /* empty the stack to run finalizers */
    luaC_freeallobjects(L);  /* collect all objects */
    luai_userstateclose(L);
  }
  luaM_freearray(L, G(L)->strt.hash, cast_sizet(G(L)->strt.size));
  freestack(L);
  lua_assert(gettotalbytes(g) == आकार(global_State));
  (*g->frealloc)(g->ud, g, आकार(global_State), 0);  /* free main block */
}


LUA_API lua_State *lua_newthread (lua_State *L) {
  global_State *g = G(L);
  GCObject *o;
  lua_State *L1;
  lua_lock(L);
  luaC_checkGC(L);
  /* create new thread */
  o = luaC_newobjdt(L, LUA_TTHREAD, आकार(LX), दुरत्व(LX, l));
  L1 = gco2th(o);
  /* anchor it on L stack */
  setthvalue2s(L, L->शीर्ष.p, L1);
  api_incr_शीर्ष(L);
  preinit_thread(L1, g);
  L1->hookmask = L->hookmask;
  L1->basehookcount = L->basehookcount;
  L1->hook = L->hook;
  resethookcount(L1);
  /* initialize L1 extra space */
  स_नकल(lua_getextraspace(L1), lua_getextraspace(mainthread(g)),
         LUA_EXTRASPACE);
  luai_userstatethread(L, L1);
  चळत_init(L1, L);  /* init stack */
  lua_unlock(L);
  परत L1;
}


शून्य luaE_freethread (lua_State *L, lua_State *L1) {
  LX *l = fromstate(L1);
  luaF_closeupval(L1, L1->चळत.p);  /* close all upvalues */
  lua_assert(L1->openupval == रिक्त);
  luai_userstatefree(L, L1);
  freestack(L1);
  luaM_free(L, l);
}


TStatus luaE_resetthread (lua_State *L, TStatus status) {
  resetCI(L);
  जर (status == LUA_YIELD)
    status = LUA_OK;
  status = luaD_closeprotected(L, 1, status);
  जर (status != LUA_OK)  /* errors? */
    luaD_seterrorobj(L, status, L->चळत.p + 1);
  अन्यथा  /* keep in the stack only the fake function-entry for basic ci */
    L->शीर्ष.p = L->चळत.p + 1;
  luaD_reallocstack(L, cast_int(L->ci->शीर्ष.p - L->चळत.p), 0);
  परत status;
}


/*
** Auxiliary function to call luaE_resetthread in protected mode.
** That function closes all tbc variables in protected mode, so the
** only possible "error" is when a tbc variable closes the thread being
** closed and therefore calls 'luaD_throwbaselevel'; in that case,
** control goes directly back to the first call to lua_closethread (not
** to the second), to properly return it.
*/
स्थिर शून्य resetthread_aux (lua_State *L, शून्य *ud) {
  TStatus status = *cast(TStatus*, ud);
  status = luaE_resetthread(L, status);
  जर (status != LUA_OK)
    luaD_throw(L, status);
}


LUA_API पूर्णांक lua_closethread (lua_State *L, lua_State *from) {
  TStatus status;
  lua_lock(L);
  L->nCcalls = (from) ? getCcalls(from) : 0;
  status = luaD_rawrunprotected(L, resetthread_aux, &L->status);
  /* stack has error message iff there was an error */
  lua_assert(L->शीर्ष.p == L->चळत.p + (status == LUA_OK ? 1 : 2));
  जर (L == from)  /* closing itself? */
    luaD_throwbaselevel(L, status);
  lua_unlock(L);
  परत APIstatus(status);
}


LUA_API lua_State *lua_newstate (lua_Alloc f, शून्य *ud, अचिन्हित seed) {
  पूर्णांक i;
  lua_State *L;
  global_State *g = cast(global_State*,
                       (*f)(ud, रिक्त, LUA_TTHREAD, आकार(global_State)));
  जर (g == रिक्त) परत रिक्त;
  L = &g->mainth.l;
  L->tt = LUA_VTHREAD;
  g->currentwhite = bitmask(WHITE0BIT);
  L->marked = luaC_white(g);
  preinit_thread(L, g);
  g->allgc = obj2gco(L);  /* by now, only object is the main thread */
  L->next = रिक्त;
  incnny(L);  /* main thread is always non yieldable */
  g->frealloc = f;
  g->ud = ud;
  g->warnf = रिक्त;
  g->ud_warn = रिक्त;
  g->seed = seed;
  g->gcstp = GCSTPGC;  /* no GC while building state */
  g->strt.size = g->strt.nuse = 0;
  g->strt.hash = रिक्त;
  setnilvalue(&g->l_registry);
  g->panic = रिक्त;
  g->gcstate = GCSpause;
  g->gckind = KGC_INC;
  g->gcstopem = 0;
  g->gcemergency = 0;
  g->finobj = g->tobefnz = g->fixedgc = रिक्त;
  g->firstold1 = g->survival = g->old1 = g->reallyold = रिक्त;
  g->finobjsur = g->finobjold1 = g->finobjrold = रिक्त;
  g->sweepgc = रिक्त;
  g->gray = g->grayagain = रिक्त;
  g->weak = g->ephemeron = g->allweak = रिक्त;
  g->twups = रिक्त;
  g->GCtotalbytes = आकार(global_State);
  g->GCmarked = 0;
  g->GCdebt = 0;
  setivalue(&g->nilvalue, 0);  /* to signal that state is not yet built */
  setgcparam(g, PAUSE, LUAI_GCPAUSE);
  setgcparam(g, STEPMUL, LUAI_GCMUL);
  setgcparam(g, STEPSIZE, LUAI_GCSTEPSIZE);
  setgcparam(g, MINORMUL, LUAI_GENMINORMUL);
  setgcparam(g, MINORMAJOR, LUAI_MINORMAJOR);
  setgcparam(g, MAJORMINOR, LUAI_MAJORMINOR);
  साठी (i=0; i < LUA_NUMTYPES; i++) g->mt[i] = रिक्त;
  जर (luaD_rawrunprotected(L, f_luaopen, रिक्त) != LUA_OK) {
    /* memory allocation error: free partial state */
    close_state(L);
    L = रिक्त;
  }
  परत L;
}


LUA_API शून्य lua_close (lua_State *L) {
  lua_lock(L);
  L = mainthread(G(L));  /* only the main thread can be closed */
  close_state(L);
}


शून्य luaE_warning (lua_State *L, स्थिरांक अक्षर *संदेश, पूर्णांक tocont) {
  lua_WarnFunction wf = G(L)->warnf;
  जर (wf != रिक्त)
    wf(G(L)->ud_warn, संदेश, tocont);
}


/*
** Generate a warning from an error message
*/
शून्य luaE_warnerror (lua_State *L, स्थिरांक अक्षर *where) {
  TValue *errobj = s2v(L->शीर्ष.p - 1);  /* error object */
  स्थिरांक अक्षर *संदेश = (ttisstring(errobj))
                  ? getstr(tsvalue(errobj))
                  : "error object is not a string";
  /* produce warning "error in %s (%s)" (where, msg) */
  luaE_warning(L, "error in ", 1);
  luaE_warning(L, where, 1);
  luaE_warning(L, " (", 1);
  luaE_warning(L, संदेश, 1);
  luaE_warning(L, ")", 0);
}

