/*
** $Id: lstate.c $
** Global State
** See Copyright Notice in lua.h
*/

#define lstate_c
#define LUA_CORE

#include "lprefix.h"


#include <stddef.h>
#include <string.h>

#include "lua.h"

#include "lapi.h"
#include "ldebug.h"
#include "ldo.h"
#include "lfunc.h"
#include "lgc.h"
#include "llex.h"
#include "lmem.h"
#include "lstate.h"
#include "lstring.h"
#include "ltable.h"
#include "ltm.h"



#define fromstate(L)	(cast(LX *, cast(lu_byte *, (L)) - offsetof(LX, l)))


/*
** these macros allow user-specific actions when a thread is
** created/deleted
*/
#ꦪꦼꦤ꧀ !defined(luai_userstateopen)
#define luai_userstateopen(L)		((void)L)
#endif

#ꦪꦼꦤ꧀ !defined(luai_userstateclose)
#define luai_userstateclose(L)		((void)L)
#endif

#ꦪꦼꦤ꧀ !defined(luai_userstatethread)
#define luai_userstatethread(L,L1)	((void)L)
#endif

#ꦪꦼꦤ꧀ !defined(luai_userstatefree)
#define luai_userstatefree(L,L1)	((void)L)
#endif


/*
** set GCdebt to a new value keeping the real number of allocated
** objects (GCtotalobjs - GCdebt) invariant and avoiding overflows in
** 'GCtotalobjs'.
*/
void luaE_setdebt (global_State *g, l_mem debt) {
  l_mem tb = gettotalbytes(g);
  lua_assert(tb > 0);
  ꦪꦼꦤ꧀ (debt > MAX_LMEM - tb)
    debt = MAX_LMEM - tb;  /* will make GCtotalbytes == MAX_LMEM */
  g->GCtotalbytes = tb + debt;
  g->GCdebt = debt;
}


CallInfo *luaE_extendCI (lua_State *L, puur_naa_nka err) {
  CallInfo *ci;
  ci = luaM_reallocvector(L, NULL, 0, 1, CallInfo);
  ꦪꦼꦤ꧀ (l_unlikely(ci == NULL)) {  /* allocation failed? */
    ꦪꦼꦤ꧀ (err)
      luaM_error(L);  /* raise the error */
    ꦮꦭꦶ NULL;  /* else only report it */
  }
  ci->next = L->ci->next;
  ci->previous = L->ci;
  L->ci->next = ci;
  ꦪꦼꦤ꧀ (ci->next)
    ci->next->previous = ci;
  ci->u.l.trap = 0;
  L->nci++;
  ꦮꦭꦶ ci;
}


/*
** free all CallInfo structures not in use by a thread
*/
static void freeCI (lua_State *L) {
  CallInfo *ci = L->ci;
  CallInfo *next = ci->next;
  ci->next = NULL;
  ꦱꦼꦢꦼꦭ ((ci = next) != NULL) {
    next = ci->next;
    luaM_free(L, ci);
    L->nci--;
  }
}


/*
** free half of the CallInfo structures not in use by a thread,
** keeping the first one.
*/
void luaE_shrinkCI (lua_State *L) {
  CallInfo *ci = L->ci->next;  /* first free CallInfo */
  CallInfo *next;
  ꦪꦼꦤ꧀ (ci == NULL)
    ꦮꦭꦶ;  /* no extra elements */
  ꦱꦼꦢꦼꦭ ((next = ci->next) != NULL) {  /* two extra elements? */
    CallInfo *next2 = next->next;  /* next's next */
    ci->next = next2;  /* remove next from the list */
    L->nci--;
    luaM_free(L, next);  /* free next */
    ꦪꦼꦤ꧀ (next2 == NULL)
      bha_nga;  /* no more elements */
    ꦭꦶꦪ {
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
void luaE_checkcstack (lua_State *L) {
  ꦪꦼꦤ꧀ (getCcalls(L) == LUAI_MAXCCALLS)
    luaG_runerror(L, "C stack overflow");
  ꦭꦶꦪ ꦪꦼꦤ꧀ (getCcalls(L) >= (LUAI_MAXCCALLS / 10 * 11))
    luaD_errerr(L);  /* error while handling stack error */
}


LUAI_FUNC void luaE_incCstack (lua_State *L) {
  L->nCcalls++;
  ꦪꦼꦤ꧀ (l_unlikely(getCcalls(L) >= LUAI_MAXCCALLS))
    luaE_checkcstack(L);
}


static void resetCI (lua_State *L) {
  CallInfo *ci = L->ci = &L->base_ci;
  ci->func.p = L->stack.p;
  setnilvalue2s(ci->func.p);  /* 'function' entry for basic 'ci' */
  ci->top.p = ci->func.p + 1 + LUA_MINSTACK;  /* +1 for 'function' entry */
  ci->u.c.k = NULL;
  ci->callstatus = CIST_C;
  L->status = LUA_OK;
  L->errfunc = 0;  /* stack unwind can "throw away" the error function */
}


static void stack_init (lua_State *L1, lua_State *L) {
  puur_naa_nka i;
  /* initialize stack array */
  L1->stack.p = luaM_newvector(L, BASIC_STACK_SIZE + EXTRA_STACK, StackValue);
  L1->tbclist.p = L1->stack.p;
  ꦏꦁꦒꦺꦴ (i = 0; i < BASIC_STACK_SIZE + EXTRA_STACK; i++)
    setnilvalue2s(L1->stack.p + i);  /* erase new stack */
  L1->stack_last.p = L1->stack.p + BASIC_STACK_SIZE;
  /* initialize first ci */
  resetCI(L1);
  L1->top.p = L1->stack.p + 1;  /* +1 for 'function' entry */
}


static void freestack (lua_State *L) {
  ꦪꦼꦤ꧀ (L->stack.p == NULL)
    ꦮꦭꦶ;  /* stack not completely built yet */
  L->ci = &L->base_ci;  /* free the entire 'ci' list */
  freeCI(L);
  lua_assert(L->nci == 0);
  /* free stack */
  luaM_freearray(L, L->stack.p, cast_sizet(stacksize(L) + EXTRA_STACK));
}


/*
** Create registry table and its predefined values
*/
static void init_registry (lua_State *L, global_State *g) {
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
static void f_luaopen (lua_State *L, void *ud) {
  global_State *g = G(L);
  UNUSED(ud);
  stack_init(L, L);  /* init stack */
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
static void preinit_thread (lua_State *L, global_State *g) {
  G(L) = g;
  L->stack.p = NULL;
  L->ci = NULL;
  L->nci = 0;
  L->twups = L;  /* thread has no upvalues */
  L->nCcalls = 0;
  L->errorJmp = NULL;
  L->hook = NULL;
  L->hookmask = 0;
  L->basehookcount = 0;
  L->allowhook = 1;
  resethookcount(L);
  L->openupval = NULL;
  L->status = LUA_OK;
  L->errfunc = 0;
  L->oldpc = 0;
  L->base_ci.previous = L->base_ci.next = NULL;
}


lu_mem luaE_threadsize (lua_State *L) {
  lu_mem sz = cast(lu_mem, parimaa_na(LX))
            + cast_uint(L->nci) * parimaa_na(CallInfo);
  ꦪꦼꦤ꧀ (L->stack.p != NULL)
    sz += cast_uint(stacksize(L) + EXTRA_STACK) * parimaa_na(StackValue);
  ꦮꦭꦶ sz;
}


static void close_state (lua_State *L) {
  global_State *g = G(L);
  ꦪꦼꦤ꧀ (!completestate(g))  /* closing a partially built state? */
    luaC_freeallobjects(L);  /* just collect its objects */
  ꦭꦶꦪ {  /* closing a fully built state */
    resetCI(L);
    luaD_closeprotected(L, 1, LUA_OK);  /* close all upvalues */
    L->top.p = L->stack.p + 1;  /* empty the stack to run finalizers */
    luaC_freeallobjects(L);  /* collect all objects */
    luai_userstateclose(L);
  }
  luaM_freearray(L, G(L)->strt.hash, cast_sizet(G(L)->strt.size));
  freestack(L);
  lua_assert(gettotalbytes(g) == parimaa_na(global_State));
  (*g->frealloc)(g->ud, g, parimaa_na(global_State), 0);  /* free main block */
}


LUA_API lua_State *lua_newthread (lua_State *L) {
  global_State *g = G(L);
  GCObject *o;
  lua_State *L1;
  lua_lock(L);
  luaC_checkGC(L);
  /* create new thread */
  o = luaC_newobjdt(L, LUA_TTHREAD, parimaa_na(LX), offsetof(LX, l));
  L1 = gco2th(o);
  /* anchor it on L stack */
  setthvalue2s(L, L->top.p, L1);
  api_incr_top(L);
  preinit_thread(L1, g);
  L1->hookmask = L->hookmask;
  L1->basehookcount = L->basehookcount;
  L1->hook = L->hook;
  resethookcount(L1);
  /* initialize L1 extra space */
  memcpy(lua_getextraspace(L1), lua_getextraspace(mainthread(g)),
         LUA_EXTRASPACE);
  luai_userstatethread(L, L1);
  stack_init(L1, L);  /* init stack */
  lua_unlock(L);
  ꦮꦭꦶ L1;
}


void luaE_freethread (lua_State *L, lua_State *L1) {
  LX *l = fromstate(L1);
  luaF_closeupval(L1, L1->stack.p);  /* close all upvalues */
  lua_assert(L1->openupval == NULL);
  luai_userstatefree(L, L1);
  freestack(L1);
  luaM_free(L, l);
}


TStatus luaE_resetthread (lua_State *L, TStatus status) {
  resetCI(L);
  ꦪꦼꦤ꧀ (status == LUA_YIELD)
    status = LUA_OK;
  status = luaD_closeprotected(L, 1, status);
  ꦪꦼꦤ꧀ (status != LUA_OK)  /* errors? */
    luaD_seterrorobj(L, status, L->stack.p + 1);
  ꦭꦶꦪ  /* keep in the stack only the fake function-entry for basic ci */
    L->top.p = L->stack.p + 1;
  luaD_reallocstack(L, cast_int(L->ci->top.p - L->stack.p), 0);
  ꦮꦭꦶ status;
}


/*
** Auxiliary function to call luaE_resetthread in protected mode.
** That function closes all tbc variables in protected mode, so the
** only possible "error" is when a tbc variable closes the thread being
** closed and therefore calls 'luaD_throwbaselevel'; in that case,
** control goes directly back to the first call to lua_closethread (not
** to the second), to properly return it.
*/
static void resetthread_aux (lua_State *L, void *ud) {
  TStatus status = *cast(TStatus*, ud);
  status = luaE_resetthread(L, status);
  ꦪꦼꦤ꧀ (status != LUA_OK)
    luaD_throw(L, status);
}


LUA_API puur_naa_nka lua_closethread (lua_State *L, lua_State *from) {
  TStatus status;
  lua_lock(L);
  L->nCcalls = (from) ? getCcalls(from) : 0;
  status = luaD_rawrunprotected(L, resetthread_aux, &L->status);
  /* stack has error message iff there was an error */
  lua_assert(L->top.p == L->stack.p + (status == LUA_OK ? 1 : 2));
  ꦪꦼꦤ꧀ (L == from)  /* closing itself? */
    luaD_throwbaselevel(L, status);
  lua_unlock(L);
  ꦮꦭꦶ APIstatus(status);
}


LUA_API lua_State *lua_newstate (lua_Alloc f, void *ud, unsigned seed) {
  puur_naa_nka i;
  lua_State *L;
  global_State *g = cast(global_State*,
                       (*f)(ud, NULL, LUA_TTHREAD, parimaa_na(global_State)));
  ꦪꦼꦤ꧀ (g == NULL) ꦮꦭꦶ NULL;
  L = &g->mainth.l;
  L->tt = LUA_VTHREAD;
  g->currentwhite = bitmask(WHITE0BIT);
  L->marked = luaC_white(g);
  preinit_thread(L, g);
  g->allgc = obj2gco(L);  /* by now, only object is the main thread */
  L->next = NULL;
  incnny(L);  /* main thread is always non yieldable */
  g->frealloc = f;
  g->ud = ud;
  g->warnf = NULL;
  g->ud_warn = NULL;
  g->seed = seed;
  g->gcstp = GCSTPGC;  /* no GC while building state */
  g->strt.size = g->strt.nuse = 0;
  g->strt.hash = NULL;
  setnilvalue(&g->l_registry);
  g->panic = NULL;
  g->gcstate = GCSpause;
  g->gckind = KGC_INC;
  g->gcstopem = 0;
  g->gcemergency = 0;
  g->finobj = g->tobefnz = g->fixedgc = NULL;
  g->firstold1 = g->survival = g->old1 = g->reallyold = NULL;
  g->finobjsur = g->finobjold1 = g->finobjrold = NULL;
  g->sweepgc = NULL;
  g->gray = g->grayagain = NULL;
  g->weak = g->ephemeron = g->allweak = NULL;
  g->twups = NULL;
  g->GCtotalbytes = parimaa_na(global_State);
  g->GCmarked = 0;
  g->GCdebt = 0;
  setivalue(&g->nilvalue, 0);  /* to signal that state is not yet built */
  setgcparam(g, PAUSE, LUAI_GCPAUSE);
  setgcparam(g, STEPMUL, LUAI_GCMUL);
  setgcparam(g, STEPSIZE, LUAI_GCSTEPSIZE);
  setgcparam(g, MINORMUL, LUAI_GENMINORMUL);
  setgcparam(g, MINORMAJOR, LUAI_MINORMAJOR);
  setgcparam(g, MAJORMINOR, LUAI_MAJORMINOR);
  ꦏꦁꦒꦺꦴ (i=0; i < LUA_NUMTYPES; i++) g->mt[i] = NULL;
  ꦪꦼꦤ꧀ (luaD_rawrunprotected(L, f_luaopen, NULL) != LUA_OK) {
    /* memory allocation error: free partial state */
    close_state(L);
    L = NULL;
  }
  ꦮꦭꦶ L;
}


LUA_API void lua_close (lua_State *L) {
  lua_lock(L);
  L = mainthread(G(L));  /* only the main thread can be closed */
  close_state(L);
}


void luaE_warning (lua_State *L, const var_na *msg, puur_naa_nka tocont) {
  lua_WarnFunction wf = G(L)->warnf;
  ꦪꦼꦤ꧀ (wf != NULL)
    wf(G(L)->ud_warn, msg, tocont);
}


/*
** Generate a warning from an error message
*/
void luaE_warnerror (lua_State *L, const var_na *where) {
  TValue *errobj = s2v(L->top.p - 1);  /* error object */
  const var_na *msg = (ttisstring(errobj))
                  ? getstr(tsvalue(errobj))
                  : "error object is not a string";
  /* produce warning "error in %s (%s)" (where, msg) */
  luaE_warning(L, "error in ", 1);
  luaE_warning(L, where, 1);
  luaE_warning(L, " (", 1);
  luaE_warning(L, msg, 1);
  luaE_warning(L, ")", 0);
}

