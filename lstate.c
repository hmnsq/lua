/*
** $Id: lstate.c $
** Global State
** See Copyright Notice in lua.h
*/

#define lstate_c
#define LUA_CORE

#അന്തര്ഭാവ "lprefix.h"


#അന്തര്ഭാവ <stddef.h>
#അന്തര്ഭാവ <string.h>

#അന്തര്ഭാവ "lua.h"

#അന്തര്ഭാവ "lapi.h"
#അന്തര്ഭാവ "ldebug.h"
#അന്തര്ഭാവ "ldo.h"
#അന്തര്ഭാവ "lfunc.h"
#അന്തര്ഭാവ "lgc.h"
#അന്തര്ഭാവ "llex.h"
#അന്തര്ഭാവ "lmem.h"
#അന്തര്ഭാവ "lstate.h"
#അന്തര്ഭാവ "lstring.h"
#അന്തര്ഭാവ "ltable.h"
#അന്തര്ഭാവ "ltm.h"



#define fromstate(L)	(cast(LX *, cast(lu_byte *, (L)) - offsetof(LX, l)))


/*
** these macros allow user-specific actions when a thread is
** created/deleted
*/
#എങ്കിൽ !defined(luai_userstateopen)
#define luai_userstateopen(L)		((ശൂന്യം)L)
#endif

#എങ്കിൽ !defined(luai_userstateclose)
#define luai_userstateclose(L)		((ശൂന്യം)L)
#endif

#എങ്കിൽ !defined(luai_userstatethread)
#define luai_userstatethread(L,L1)	((ശൂന്യം)L)
#endif

#എങ്കിൽ !defined(luai_userstatefree)
#define luai_userstatefree(L,L1)	((ശൂന്യം)L)
#endif


/*
** set GCdebt to a new value keeping the real number of allocated
** objects (GCtotalobjs - GCdebt) invariant and avoiding overflows in
** 'GCtotalobjs'.
*/
ശൂന്യം luaE_setdebt (global_State *g, l_mem debt) {
  l_mem tb = gettotalbytes(g);
  lua_assert(tb > 0);
  എങ്കിൽ (debt > MAX_LMEM - tb)
    debt = MAX_LMEM - tb;  /* will make GCtotalbytes == MAX_LMEM */
  g->GCtotalbytes = tb + debt;
  g->GCdebt = debt;
}


CallInfo *luaE_extendCI (lua_State *L, പൂർണ്ണസംഖ്യ err) {
  CallInfo *ci;
  ci = luaM_reallocvector(L, ശൂന്യ, 0, 1, CallInfo);
  എങ്കിൽ (l_unlikely(ci == ശൂന്യ)) {  /* allocation failed? */
    എങ്കിൽ (err)
      luaM_error(L);  /* raise the error */
    തിരികെ ശൂന്യ;  /* else only report it */
  }
  ci->next = L->ci->next;
  ci->previous = L->ci;
  L->ci->next = ci;
  എങ്കിൽ (ci->next)
    ci->next->previous = ci;
  ci->u.l.trap = 0;
  L->nci++;
  തിരികെ ci;
}


/*
** free all CallInfo structures not in use by a thread
*/
സ്ഥിതം ശൂന്യം freeCI (lua_State *L) {
  CallInfo *ci = L->ci;
  CallInfo *next = ci->next;
  ci->next = ശൂന്യ;
  ആകുമ്പോൾ ((ci = next) != ശൂന്യ) {
    next = ci->next;
    luaM_free(L, ci);
    L->nci--;
  }
}


/*
** free half of the CallInfo structures not in use by a thread,
** keeping the first one.
*/
ശൂന്യം luaE_shrinkCI (lua_State *L) {
  CallInfo *ci = L->ci->next;  /* first free CallInfo */
  CallInfo *next;
  എങ്കിൽ (ci == ശൂന്യ)
    തിരികെ;  /* no extra elements */
  ആകുമ്പോൾ ((next = ci->next) != ശൂന്യ) {  /* two extra elements? */
    CallInfo *next2 = next->next;  /* next's next */
    ci->next = next2;  /* remove next from the list */
    L->nci--;
    luaM_free(L, next);  /* free next */
    എങ്കിൽ (next2 == ശൂന്യ)
      നിർത്തുക;  /* no more elements */
    അല്ലെങ്കിൽ {
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
ശൂന്യം luaE_checkcstack (lua_State *L) {
  എങ്കിൽ (getCcalls(L) == LUAI_MAXCCALLS)
    luaG_runerror(L, "C stack overflow");
  അല്ലെങ്കിൽ എങ്കിൽ (getCcalls(L) >= (LUAI_MAXCCALLS / 10 * 11))
    luaD_errerr(L);  /* error while handling stack error */
}


LUAI_FUNC ശൂന്യം luaE_incCstack (lua_State *L) {
  L->nCcalls++;
  എങ്കിൽ (l_unlikely(getCcalls(L) >= LUAI_MAXCCALLS))
    luaE_checkcstack(L);
}


സ്ഥിതം ശൂന്യം resetCI (lua_State *L) {
  CallInfo *ci = L->ci = &L->base_ci;
  ci->func.p = L->അട്ടി.p;
  setnilvalue2s(ci->func.p);  /* 'function' entry for basic 'ci' */
  ci->മുകൾ.p = ci->func.p + 1 + LUA_MINSTACK;  /* +1 for 'function' entry */
  ci->u.c.k = ശൂന്യ;
  ci->callstatus = CIST_C;
  L->status = LUA_OK;
  L->errfunc = 0;  /* stack unwind can "throw away" the error function */
}


സ്ഥിതം ശൂന്യം അട്ടി_init (lua_State *L1, lua_State *L) {
  പൂർണ്ണസംഖ്യ i;
  /* initialize stack array */
  L1->അട്ടി.p = luaM_newvector(L, BASIC_അട്ടി_SIZE + EXTRA_അട്ടി, അട്ടി_മൂല്യം);
  L1->tbclist.p = L1->അട്ടി.p;
  വേണ്ടി (i = 0; i < BASIC_അട്ടി_SIZE + EXTRA_അട്ടി; i++)
    setnilvalue2s(L1->അട്ടി.p + i);  /* erase new stack */
  L1->അട്ടി_last.p = L1->അട്ടി.p + BASIC_അട്ടി_SIZE;
  /* initialize first ci */
  resetCI(L1);
  L1->മുകൾ.p = L1->അട്ടി.p + 1;  /* +1 for 'function' entry */
}


സ്ഥിതം ശൂന്യം freestack (lua_State *L) {
  എങ്കിൽ (L->അട്ടി.p == ശൂന്യ)
    തിരികെ;  /* stack not completely built yet */
  L->ci = &L->base_ci;  /* free the entire 'ci' list */
  freeCI(L);
  lua_assert(L->nci == 0);
  /* free stack */
  luaM_freearray(L, L->അട്ടി.p, cast_sizet(stacksize(L) + EXTRA_അട്ടി));
}


/*
** Create registry table and its predefined values
*/
സ്ഥിതം ശൂന്യം init_registry (lua_State *L, global_State *g) {
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
സ്ഥിതം ശൂന്യം f_luaopen (lua_State *L, ശൂന്യം *ud) {
  global_State *g = G(L);
  UNUSED(ud);
  അട്ടി_init(L, L);  /* init stack */
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
സ്ഥിതം ശൂന്യം preinit_thread (lua_State *L, global_State *g) {
  G(L) = g;
  L->അട്ടി.p = ശൂന്യ;
  L->ci = ശൂന്യ;
  L->nci = 0;
  L->twups = L;  /* thread has no upvalues */
  L->nCcalls = 0;
  L->errorJmp = ശൂന്യ;
  L->hook = ശൂന്യ;
  L->hookmask = 0;
  L->basehookcount = 0;
  L->allowhook = 1;
  resethookcount(L);
  L->openupval = ശൂന്യ;
  L->status = LUA_OK;
  L->errfunc = 0;
  L->oldpc = 0;
  L->base_ci.previous = L->base_ci.next = ശൂന്യ;
}


lu_mem luaE_threadsize (lua_State *L) {
  lu_mem sz = cast(lu_mem, വലുപ്പം(LX))
            + cast_uint(L->nci) * വലുപ്പം(CallInfo);
  എങ്കിൽ (L->അട്ടി.p != ശൂന്യ)
    sz += cast_uint(stacksize(L) + EXTRA_അട്ടി) * വലുപ്പം(അട്ടി_മൂല്യം);
  തിരികെ sz;
}


സ്ഥിതം ശൂന്യം close_state (lua_State *L) {
  global_State *g = G(L);
  എങ്കിൽ (!completestate(g))  /* closing a partially built state? */
    luaC_freeallobjects(L);  /* just collect its objects */
  അല്ലെങ്കിൽ {  /* closing a fully built state */
    resetCI(L);
    luaD_closeprotected(L, 1, LUA_OK);  /* close all upvalues */
    L->മുകൾ.p = L->അട്ടി.p + 1;  /* empty the stack to run finalizers */
    luaC_freeallobjects(L);  /* collect all objects */
    luai_userstateclose(L);
  }
  luaM_freearray(L, G(L)->strt.hash, cast_sizet(G(L)->strt.size));
  freestack(L);
  lua_assert(gettotalbytes(g) == വലുപ്പം(global_State));
  (*g->frealloc)(g->ud, g, വലുപ്പം(global_State), 0);  /* free main block */
}


LUA_API lua_State *lua_newthread (lua_State *L) {
  global_State *g = G(L);
  GCObject *o;
  lua_State *L1;
  lua_lock(L);
  luaC_checkGC(L);
  /* create new thread */
  o = luaC_newobjdt(L, LUA_TTHREAD, വലുപ്പം(LX), offsetof(LX, l));
  L1 = gco2th(o);
  /* anchor it on L stack */
  setthvalue2s(L, L->മുകൾ.p, L1);
  api_incr_മുകൾ(L);
  preinit_thread(L1, g);
  L1->hookmask = L->hookmask;
  L1->basehookcount = L->basehookcount;
  L1->hook = L->hook;
  resethookcount(L1);
  /* initialize L1 extra space */
  memcpy(lua_getextraspace(L1), lua_getextraspace(mainthread(g)),
         LUA_EXTRASPACE);
  luai_userstatethread(L, L1);
  അട്ടി_init(L1, L);  /* init stack */
  lua_unlock(L);
  തിരികെ L1;
}


ശൂന്യം luaE_freethread (lua_State *L, lua_State *L1) {
  LX *l = fromstate(L1);
  luaF_closeupval(L1, L1->അട്ടി.p);  /* close all upvalues */
  lua_assert(L1->openupval == ശൂന്യ);
  luai_userstatefree(L, L1);
  freestack(L1);
  luaM_free(L, l);
}


TStatus luaE_resetthread (lua_State *L, TStatus status) {
  resetCI(L);
  എങ്കിൽ (status == LUA_YIELD)
    status = LUA_OK;
  status = luaD_closeprotected(L, 1, status);
  എങ്കിൽ (status != LUA_OK)  /* errors? */
    luaD_seterrorobj(L, status, L->അട്ടി.p + 1);
  അല്ലെങ്കിൽ  /* keep in the stack only the fake function-entry for basic ci */
    L->മുകൾ.p = L->അട്ടി.p + 1;
  luaD_reallocstack(L, cast_int(L->ci->മുകൾ.p - L->അട്ടി.p), 0);
  തിരികെ status;
}


/*
** Auxiliary function to call luaE_resetthread in protected mode.
** That function closes all tbc variables in protected mode, so the
** only possible "error" is when a tbc variable closes the thread being
** closed and therefore calls 'luaD_throwbaselevel'; in that case,
** control goes directly back to the first call to lua_closethread (not
** to the second), to properly return it.
*/
സ്ഥിതം ശൂന്യം resetthread_aux (lua_State *L, ശൂന്യം *ud) {
  TStatus status = *cast(TStatus*, ud);
  status = luaE_resetthread(L, status);
  എങ്കിൽ (status != LUA_OK)
    luaD_throw(L, status);
}


LUA_API പൂർണ്ണസംഖ്യ lua_closethread (lua_State *L, lua_State *from) {
  TStatus status;
  lua_lock(L);
  L->nCcalls = (from) ? getCcalls(from) : 0;
  status = luaD_rawrunprotected(L, resetthread_aux, &L->status);
  /* stack has error message iff there was an error */
  lua_assert(L->മുകൾ.p == L->അട്ടി.p + (status == LUA_OK ? 1 : 2));
  എങ്കിൽ (L == from)  /* closing itself? */
    luaD_throwbaselevel(L, status);
  lua_unlock(L);
  തിരികെ APIstatus(status);
}


LUA_API lua_State *lua_newstate (lua_Alloc f, ശൂന്യം *ud, unsigned seed) {
  പൂർണ്ണസംഖ്യ i;
  lua_State *L;
  global_State *g = cast(global_State*,
                       (*f)(ud, ശൂന്യ, LUA_TTHREAD, വലുപ്പം(global_State)));
  എങ്കിൽ (g == ശൂന്യ) തിരികെ ശൂന്യ;
  L = &g->mainth.l;
  L->tt = LUA_VTHREAD;
  g->currentwhite = bitmask(WHITE0BIT);
  L->marked = luaC_white(g);
  preinit_thread(L, g);
  g->allgc = obj2gco(L);  /* by now, only object is the main thread */
  L->next = ശൂന്യ;
  incnny(L);  /* main thread is always non yieldable */
  g->frealloc = f;
  g->ud = ud;
  g->warnf = ശൂന്യ;
  g->ud_warn = ശൂന്യ;
  g->seed = seed;
  g->gcstp = GCSTPGC;  /* no GC while building state */
  g->strt.size = g->strt.nuse = 0;
  g->strt.hash = ശൂന്യ;
  setnilvalue(&g->l_registry);
  g->panic = ശൂന്യ;
  g->gcstate = GCSpause;
  g->gckind = KGC_INC;
  g->gcstopem = 0;
  g->gcemergency = 0;
  g->finobj = g->tobefnz = g->fixedgc = ശൂന്യ;
  g->firstold1 = g->survival = g->old1 = g->reallyold = ശൂന്യ;
  g->finobjsur = g->finobjold1 = g->finobjrold = ശൂന്യ;
  g->sweepgc = ശൂന്യ;
  g->gray = g->grayagain = ശൂന്യ;
  g->weak = g->ephemeron = g->allweak = ശൂന്യ;
  g->twups = ശൂന്യ;
  g->GCtotalbytes = വലുപ്പം(global_State);
  g->GCmarked = 0;
  g->GCdebt = 0;
  setivalue(&g->nilvalue, 0);  /* to signal that state is not yet built */
  setgcparam(g, PAUSE, LUAI_GCPAUSE);
  setgcparam(g, STEPMUL, LUAI_GCMUL);
  setgcparam(g, STEPSIZE, LUAI_GCSTEPSIZE);
  setgcparam(g, MINORMUL, LUAI_GENMINORMUL);
  setgcparam(g, MINORMAJOR, LUAI_MINORMAJOR);
  setgcparam(g, MAJORMINOR, LUAI_MAJORMINOR);
  വേണ്ടി (i=0; i < LUA_NUMTYPES; i++) g->mt[i] = ശൂന്യ;
  എങ്കിൽ (luaD_rawrunprotected(L, f_luaopen, ശൂന്യ) != LUA_OK) {
    /* memory allocation error: free partial state */
    close_state(L);
    L = ശൂന്യ;
  }
  തിരികെ L;
}


LUA_API ശൂന്യം lua_close (lua_State *L) {
  lua_lock(L);
  L = mainthread(G(L));  /* only the main thread can be closed */
  close_state(L);
}


ശൂന്യം luaE_warning (lua_State *L, സ്ഥിരം അക്ഷരം *സന്ദേശം, പൂർണ്ണസംഖ്യ tocont) {
  lua_WarnFunction wf = G(L)->warnf;
  എങ്കിൽ (wf != ശൂന്യ)
    wf(G(L)->ud_warn, സന്ദേശം, tocont);
}


/*
** Generate a warning from an error message
*/
ശൂന്യം luaE_warnerror (lua_State *L, സ്ഥിരം അക്ഷരം *where) {
  TValue *errobj = s2v(L->മുകൾ.p - 1);  /* error object */
  സ്ഥിരം അക്ഷരം *സന്ദേശം = (ttisstring(errobj))
                  ? getstr(tsvalue(errobj))
                  : "error object is not a string";
  /* produce warning "error in %s (%s)" (where, msg) */
  luaE_warning(L, "error in ", 1);
  luaE_warning(L, where, 1);
  luaE_warning(L, " (", 1);
  luaE_warning(L, സന്ദേശം, 1);
  luaE_warning(L, ")", 0);
}

