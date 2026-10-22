/*
** $Id: lstate.c $
** Global State
** See Copyright Notice in lua.h
*/

#घोषणा lstate_c
#घोषणा LUA_CORE

#समावेश "lprefix.h"


#समावेश <stddef.h>
#समावेश <string.h>

#समावेश "lua.h"

#समावेश "lapi.h"
#समावेश "ldebug.h"
#समावेश "ldo.h"
#समावेश "lfunc.h"
#समावेश "lgc.h"
#समावेश "llex.h"
#समावेश "lmem.h"
#समावेश "lstate.h"
#समावेश "lstring.h"
#समावेश "ltable.h"
#समावेश "ltm.h"



#घोषणा fromstate(L)	(cast(LX *, cast(lu_byte *, (L)) - दुरत्व(LX, l)))


/*
** these macros allow user-specific actions when a thread is
** created/deleted
*/
#अगर !defined(luai_userstateopen)
#घोषणा luai_userstateopen(L)		((व्योम)L)
#पूर्ण_अगर

#अगर !defined(luai_userstateclose)
#घोषणा luai_userstateclose(L)		((व्योम)L)
#पूर्ण_अगर

#अगर !defined(luai_userstatethread)
#घोषणा luai_userstatethread(L,L1)	((व्योम)L)
#पूर्ण_अगर

#अगर !defined(luai_userstatefree)
#घोषणा luai_userstatefree(L,L1)	((व्योम)L)
#पूर्ण_अगर


/*
** set GCdebt to a new value keeping the real number of allocated
** objects (GCtotalobjs - GCdebt) invariant and avoiding overflows in
** 'GCtotalobjs'.
*/
व्योम luaE_setdebt (global_State *g, l_mem debt) {
  l_mem tb = gettotalbytes(g);
  lua_assert(tb > 0);
  अगर (debt > MAX_LMEM - tb)
    debt = MAX_LMEM - tb;  /* will make GCtotalbytes == MAX_LMEM */
  g->GCtotalbytes = tb + debt;
  g->GCdebt = debt;
}


CallInfo *luaE_extendCI (lua_State *L, पूर्णांक err) {
  CallInfo *ci;
  ci = luaM_reallocvector(L, शून्य, 0, 1, CallInfo);
  अगर (l_unlikely(ci == शून्य)) {  /* allocation failed? */
    अगर (err)
      luaM_error(L);  /* raise the error */
    वापस शून्य;  /* else only report it */
  }
  ci->अग्र = L->ci->अग्र;
  ci->previous = L->ci;
  L->ci->अग्र = ci;
  अगर (ci->अग्र)
    ci->अग्र->previous = ci;
  ci->u.l.trap = 0;
  L->nci++;
  वापस ci;
}


/*
** free all CallInfo structures not in use by a thread
*/
अटल व्योम freeCI (lua_State *L) {
  CallInfo *ci = L->ci;
  CallInfo *अग्र = ci->अग्र;
  ci->अग्र = शून्य;
  जबतक ((ci = अग्र) != शून्य) {
    अग्र = ci->अग्र;
    luaM_free(L, ci);
    L->nci--;
  }
}


/*
** free half of the CallInfo structures not in use by a thread,
** keeping the first one.
*/
व्योम luaE_shrinkCI (lua_State *L) {
  CallInfo *ci = L->ci->अग्र;  /* first free CallInfo */
  CallInfo *अग्र;
  अगर (ci == शून्य)
    वापस;  /* no extra elements */
  जबतक ((अग्र = ci->अग्र) != शून्य) {  /* two extra elements? */
    CallInfo *next2 = अग्र->अग्र;  /* next's next */
    ci->अग्र = next2;  /* remove next from the list */
    L->nci--;
    luaM_free(L, अग्र);  /* free next */
    अगर (next2 == शून्य)
      अवरोध;  /* no more elements */
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
व्योम luaE_checkcstack (lua_State *L) {
  अगर (getCcalls(L) == LUAI_MAXCCALLS)
    luaG_runerror(L, "C stack overflow");
  अन्यथा अगर (getCcalls(L) >= (LUAI_MAXCCALLS / 10 * 11))
    luaD_errerr(L);  /* error while handling stack error */
}


LUAI_FUNC व्योम luaE_incCstack (lua_State *L) {
  L->nCcalls++;
  अगर (l_unlikely(getCcalls(L) >= LUAI_MAXCCALLS))
    luaE_checkcstack(L);
}


अटल व्योम resetCI (lua_State *L) {
  CallInfo *ci = L->ci = &L->base_ci;
  ci->func.p = L->ढेर.p;
  setnilvalue2s(ci->func.p);  /* 'function' entry for basic 'ci' */
  ci->शीर्ष.p = ci->func.p + 1 + LUA_MINSTACK;  /* +1 for 'function' entry */
  ci->u.c.k = शून्य;
  ci->callstatus = CIST_C;
  L->status = LUA_OK;
  L->errfunc = 0;  /* stack unwind can "throw away" the error function */
}


अटल व्योम ढेर_init (lua_State *L1, lua_State *L) {
  पूर्णांक i;
  /* initialize stack array */
  L1->ढेर.p = luaM_newvector(L, BASIC_ढेर_आकार + EXTRA_ढेर, ढेर_मूल्य);
  L1->tbclist.p = L1->ढेर.p;
  क्रम (i = 0; i < BASIC_ढेर_आकार + EXTRA_ढेर; i++)
    setnilvalue2s(L1->ढेर.p + i);  /* erase new stack */
  L1->ढेर_last.p = L1->ढेर.p + BASIC_ढेर_आकार;
  /* initialize first ci */
  resetCI(L1);
  L1->शीर्ष.p = L1->ढेर.p + 1;  /* +1 for 'function' entry */
}


अटल व्योम freestack (lua_State *L) {
  अगर (L->ढेर.p == शून्य)
    वापस;  /* stack not completely built yet */
  L->ci = &L->base_ci;  /* free the entire 'ci' list */
  freeCI(L);
  lua_assert(L->nci == 0);
  /* free stack */
  luaM_freearray(L, L->ढेर.p, cast_sizet(stacksize(L) + EXTRA_ढेर));
}


/*
** Create registry table and its predefined values
*/
अटल व्योम init_registry (lua_State *L, global_State *g) {
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
अटल व्योम f_luaopen (lua_State *L, व्योम *ud) {
  global_State *g = G(L);
  UNUSED(ud);
  ढेर_init(L, L);  /* init stack */
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
अटल व्योम preinit_thread (lua_State *L, global_State *g) {
  G(L) = g;
  L->ढेर.p = शून्य;
  L->ci = शून्य;
  L->nci = 0;
  L->twups = L;  /* thread has no upvalues */
  L->nCcalls = 0;
  L->errorJmp = शून्य;
  L->hook = शून्य;
  L->hookmask = 0;
  L->basehookcount = 0;
  L->allowhook = 1;
  resethookcount(L);
  L->openupval = शून्य;
  L->status = LUA_OK;
  L->errfunc = 0;
  L->oldpc = 0;
  L->base_ci.previous = L->base_ci.अग्र = शून्य;
}


lu_mem luaE_threadsize (lua_State *L) {
  lu_mem sz = cast(lu_mem, माप(LX))
            + cast_uint(L->nci) * माप(CallInfo);
  अगर (L->ढेर.p != शून्य)
    sz += cast_uint(stacksize(L) + EXTRA_ढेर) * माप(ढेर_मूल्य);
  वापस sz;
}


अटल व्योम close_state (lua_State *L) {
  global_State *g = G(L);
  अगर (!completestate(g))  /* closing a partially built state? */
    luaC_freeallobjects(L);  /* just collect its objects */
  अन्यथा {  /* closing a fully built state */
    resetCI(L);
    luaD_closeprotected(L, 1, LUA_OK);  /* close all upvalues */
    L->शीर्ष.p = L->ढेर.p + 1;  /* empty the stack to run finalizers */
    luaC_freeallobjects(L);  /* collect all objects */
    luai_userstateclose(L);
  }
  luaM_freearray(L, G(L)->strt.hash, cast_sizet(G(L)->strt.आकार));
  freestack(L);
  lua_assert(gettotalbytes(g) == माप(global_State));
  (*g->frealloc)(g->ud, g, माप(global_State), 0);  /* free main block */
}


LUA_API lua_State *lua_newthread (lua_State *L) {
  global_State *g = G(L);
  GCObject *o;
  lua_State *L1;
  lua_lock(L);
  luaC_checkGC(L);
  /* create new thread */
  o = luaC_newobjdt(L, LUA_TTHREAD, माप(LX), दुरत्व(LX, l));
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
  ढेर_init(L1, L);  /* init stack */
  lua_unlock(L);
  वापस L1;
}


व्योम luaE_freethread (lua_State *L, lua_State *L1) {
  LX *l = fromstate(L1);
  luaF_closeupval(L1, L1->ढेर.p);  /* close all upvalues */
  lua_assert(L1->openupval == शून्य);
  luai_userstatefree(L, L1);
  freestack(L1);
  luaM_free(L, l);
}


TStatus luaE_resetthread (lua_State *L, TStatus status) {
  resetCI(L);
  अगर (status == LUA_YIELD)
    status = LUA_OK;
  status = luaD_closeprotected(L, 1, status);
  अगर (status != LUA_OK)  /* errors? */
    luaD_seterrorobj(L, status, L->ढेर.p + 1);
  अन्यथा  /* keep in the stack only the fake function-entry for basic ci */
    L->शीर्ष.p = L->ढेर.p + 1;
  luaD_reallocstack(L, cast_int(L->ci->शीर्ष.p - L->ढेर.p), 0);
  वापस status;
}


/*
** Auxiliary function to call luaE_resetthread in protected mode.
** That function closes all tbc variables in protected mode, so the
** only possible "error" is when a tbc variable closes the thread being
** closed and therefore calls 'luaD_throwbaselevel'; in that case,
** control goes directly back to the first call to lua_closethread (not
** to the second), to properly return it.
*/
अटल व्योम resetthread_aux (lua_State *L, व्योम *ud) {
  TStatus status = *cast(TStatus*, ud);
  status = luaE_resetthread(L, status);
  अगर (status != LUA_OK)
    luaD_throw(L, status);
}


LUA_API पूर्णांक lua_closethread (lua_State *L, lua_State *from) {
  TStatus status;
  lua_lock(L);
  L->nCcalls = (from) ? getCcalls(from) : 0;
  status = luaD_rawrunprotected(L, resetthread_aux, &L->status);
  /* stack has error message iff there was an error */
  lua_assert(L->शीर्ष.p == L->ढेर.p + (status == LUA_OK ? 1 : 2));
  अगर (L == from)  /* closing itself? */
    luaD_throwbaselevel(L, status);
  lua_unlock(L);
  वापस APIstatus(status);
}


LUA_API lua_State *lua_newstate (lua_Alloc f, व्योम *ud, अचिन्हित seed) {
  पूर्णांक i;
  lua_State *L;
  global_State *g = cast(global_State*,
                       (*f)(ud, शून्य, LUA_TTHREAD, माप(global_State)));
  अगर (g == शून्य) वापस शून्य;
  L = &g->mainth.l;
  L->tt = LUA_VTHREAD;
  g->currentwhite = bitmask(WHITE0BIT);
  L->marked = luaC_white(g);
  preinit_thread(L, g);
  g->allgc = obj2gco(L);  /* by now, only object is the main thread */
  L->अग्र = शून्य;
  incnny(L);  /* main thread is always non yieldable */
  g->frealloc = f;
  g->ud = ud;
  g->warnf = शून्य;
  g->ud_warn = शून्य;
  g->seed = seed;
  g->gcstp = GCSTPGC;  /* no GC while building state */
  g->strt.आकार = g->strt.nuse = 0;
  g->strt.hash = शून्य;
  setnilvalue(&g->l_registry);
  g->panic = शून्य;
  g->gcstate = GCSpause;
  g->gckind = KGC_INC;
  g->gcstopem = 0;
  g->gcemergency = 0;
  g->finobj = g->tobefnz = g->fixedgc = शून्य;
  g->firstold1 = g->survival = g->old1 = g->reallyold = शून्य;
  g->finobjsur = g->finobjold1 = g->finobjrold = शून्य;
  g->sweepgc = शून्य;
  g->gray = g->grayagain = शून्य;
  g->weak = g->ephemeron = g->allweak = शून्य;
  g->twups = शून्य;
  g->GCtotalbytes = माप(global_State);
  g->GCmarked = 0;
  g->GCdebt = 0;
  setivalue(&g->nilvalue, 0);  /* to signal that state is not yet built */
  setgcparam(g, PAUSE, LUAI_GCPAUSE);
  setgcparam(g, STEPMUL, LUAI_GCMUL);
  setgcparam(g, STEPSIZE, LUAI_GCSTEPSIZE);
  setgcparam(g, MINORMUL, LUAI_GENMINORMUL);
  setgcparam(g, MINORMAJOR, LUAI_MINORMAJOR);
  setgcparam(g, MAJORMINOR, LUAI_MAJORMINOR);
  क्रम (i=0; i < LUA_NUMTYPES; i++) g->mt[i] = शून्य;
  अगर (luaD_rawrunprotected(L, f_luaopen, शून्य) != LUA_OK) {
    /* memory allocation error: free partial state */
    close_state(L);
    L = शून्य;
  }
  वापस L;
}


LUA_API व्योम lua_close (lua_State *L) {
  lua_lock(L);
  L = mainthread(G(L));  /* only the main thread can be closed */
  close_state(L);
}


व्योम luaE_warning (lua_State *L, स्थिर अक्षर *संदेश, पूर्णांक tocont) {
  lua_WarnFunction wf = G(L)->warnf;
  अगर (wf != शून्य)
    wf(G(L)->ud_warn, संदेश, tocont);
}


/*
** Generate a warning from an error message
*/
व्योम luaE_warnerror (lua_State *L, स्थिर अक्षर *where) {
  TValue *errobj = s2v(L->शीर्ष.p - 1);  /* error object */
  स्थिर अक्षर *संदेश = (ttisstring(errobj))
                  ? getstr(tsvalue(errobj))
                  : "error object is not a string";
  /* produce warning "error in %s (%s)" (where, msg) */
  luaE_warning(L, "error in ", 1);
  luaE_warning(L, where, 1);
  luaE_warning(L, " (", 1);
  luaE_warning(L, संदेश, 1);
  luaE_warning(L, ")", 0);
}

