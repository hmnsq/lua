/*
** $Id: lstate.c $
** Global State
** See Copyright Notice in lua.h
*/

#تعریف_کرو lstate_c
#تعریف_کرو LUA_CORE

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



#تعریف_کرو fromstate(L)	(cast(LX *, cast(lu_byte *, (L)) - offsetof(LX, l)))


/*
** these macros allow user-specific actions when a thread is
** created/deleted
*/
#اگر !defined(luai_userstateopen)
#تعریف_کرو luai_userstateopen(L)		((خالی)L)
#اگر_ختم

#اگر !defined(luai_userstateclose)
#تعریف_کرو luai_userstateclose(L)		((خالی)L)
#اگر_ختم

#اگر !defined(luai_userstatethread)
#تعریف_کرو luai_userstatethread(L,L1)	((خالی)L)
#اگر_ختم

#اگر !defined(luai_userstatefree)
#تعریف_کرو luai_userstatefree(L,L1)	((خالی)L)
#اگر_ختم


/*
** set GCdebt to a new value keeping the real number of allocated
** objects (GCtotalobjs - GCdebt) invariant and avoiding overflows in
** 'GCtotalobjs'.
*/
خالی luaE_setdebt (global_State *g, l_mem debt) {
  l_mem tb = gettotalbytes(g);
  lua_assert(tb > 0);
  اگر (debt > MAX_LMEM - tb)
    debt = MAX_LMEM - tb;  /* will make GCtotalbytes == MAX_LMEM */
  g->GCtotalbytes = tb + debt;
  g->GCdebt = debt;
}


CallInfo *luaE_extendCI (lua_State *L, صحیح err) {
  CallInfo *ci;
  ci = luaM_reallocvector(L, تہی, 0, 1, CallInfo);
  اگر (l_unlikely(ci == تہی)) {  /* allocation failed? */
    اگر (err)
      luaM_error(L);  /* raise the error */
    واپس تہی;  /* else only report it */
  }
  ci->اگلا = L->ci->اگلا;
  ci->پچھلا = L->ci;
  L->ci->اگلا = ci;
  اگر (ci->اگلا)
    ci->اگلا->پچھلا = ci;
  ci->u.l.trap = 0;
  L->nci++;
  واپس ci;
}


/*
** free all CallInfo structures not in use by a thread
*/
ساکن خالی freeCI (lua_State *L) {
  CallInfo *ci = L->ci;
  CallInfo *اگلا = ci->اگلا;
  ci->اگلا = تہی;
  جب ((ci = اگلا) != تہی) {
    اگلا = ci->اگلا;
    luaM_free(L, ci);
    L->nci--;
  }
}


/*
** free half of the CallInfo structures not in use by a thread,
** keeping the first one.
*/
خالی luaE_shrinkCI (lua_State *L) {
  CallInfo *ci = L->ci->اگلا;  /* first free CallInfo */
  CallInfo *اگلا;
  اگر (ci == تہی)
    واپس;  /* no extra elements */
  جب ((اگلا = ci->اگلا) != تہی) {  /* two extra elements? */
    CallInfo *next2 = اگلا->اگلا;  /* next's next */
    ci->اگلا = next2;  /* remove next from the list */
    L->nci--;
    luaM_free(L, اگلا);  /* free next */
    اگر (next2 == تہی)
      توڑو;  /* no more elements */
    ورنہ {
      next2->پچھلا = ci;
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
خالی luaE_checkcstack (lua_State *L) {
  اگر (getCcalls(L) == LUAI_MAXCCALLS)
    luaG_runerror(L, "C stack overflow");
  ورنہ اگر (getCcalls(L) >= (LUAI_MAXCCALLS / 10 * 11))
    luaD_errerr(L);  /* error while handling stack error */
}


LUAI_FUNC خالی luaE_incCstack (lua_State *L) {
  L->nCcalls++;
  اگر (l_unlikely(getCcalls(L) >= LUAI_MAXCCALLS))
    luaE_checkcstack(L);
}


ساکن خالی resetCI (lua_State *L) {
  CallInfo *ci = L->ci = &L->بنیاد_ci;
  ci->func.p = L->ڈھیر.p;
  setnilvalue2s(ci->func.p);  /* 'function' entry for basic 'ci' */
  ci->چوٹی.p = ci->func.p + 1 + LUA_MINSTACK;  /* +1 for 'function' entry */
  ci->u.c.k = تہی;
  ci->callstatus = CIST_C;
  L->حالت_2 = LUA_OK;
  L->errfunc = 0;  /* stack unwind can "throw away" the error function */
}


ساکن خالی ڈھیر_init (lua_State *L1, lua_State *L) {
  صحیح i;
  /* initialize stack array */
  L1->ڈھیر.p = luaM_newvector(L, BASIC_ڈھیر_سائز + EXTRA_ڈھیر, ڈھیر_قدر);
  L1->tbclist.p = L1->ڈھیر.p;
  برائے (i = 0; i < BASIC_ڈھیر_سائز + EXTRA_ڈھیر; i++)
    setnilvalue2s(L1->ڈھیر.p + i);  /* erase new stack */
  L1->ڈھیر_آخری.p = L1->ڈھیر.p + BASIC_ڈھیر_سائز;
  /* initialize first ci */
  resetCI(L1);
  L1->چوٹی.p = L1->ڈھیر.p + 1;  /* +1 for 'function' entry */
}


ساکن خالی freestack (lua_State *L) {
  اگر (L->ڈھیر.p == تہی)
    واپس;  /* stack not completely built yet */
  L->ci = &L->بنیاد_ci;  /* free the entire 'ci' list */
  freeCI(L);
  lua_assert(L->nci == 0);
  /* free stack */
  luaM_freearray(L, L->ڈھیر.p, cast_sizet(stacksize(L) + EXTRA_ڈھیر));
}


/*
** Create registry table and its predefined values
*/
ساکن خالی init_registry (lua_State *L, global_State *g) {
  /* create registry */
  TValue aux;
  جدول *registry = luaH_new(L);
  sethvalue(L, &g->l_registry, registry);
  luaH_resize(L, registry, LUA_RIDX_آخری, 0);
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
ساکن خالی f_luaopen (lua_State *L, خالی *ud) {
  global_State *g = G(L);
  غیر_مستعمل(ud);
  ڈھیر_init(L, L);  /* init stack */
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
ساکن خالی preinit_thread (lua_State *L, global_State *g) {
  G(L) = g;
  L->ڈھیر.p = تہی;
  L->ci = تہی;
  L->nci = 0;
  L->twups = L;  /* thread has no upvalues */
  L->nCcalls = 0;
  L->errorJmp = تہی;
  L->hook = تہی;
  L->hookmask = 0;
  L->basehookcount = 0;
  L->allowhook = 1;
  resethookcount(L);
  L->openupval = تہی;
  L->حالت_2 = LUA_OK;
  L->errfunc = 0;
  L->oldpc = 0;
  L->بنیاد_ci.پچھلا = L->بنیاد_ci.اگلا = تہی;
}


lu_mem luaE_threadsize (lua_State *L) {
  lu_mem sz = cast(lu_mem, حجم(LX))
            + cast_uint(L->nci) * حجم(CallInfo);
  اگر (L->ڈھیر.p != تہی)
    sz += cast_uint(stacksize(L) + EXTRA_ڈھیر) * حجم(ڈھیر_قدر);
  واپس sz;
}


ساکن خالی close_state (lua_State *L) {
  global_State *g = G(L);
  اگر (!completestate(g))  /* closing a partially built state? */
    luaC_freeallobjects(L);  /* just collect its objects */
  ورنہ {  /* closing a fully built state */
    resetCI(L);
    luaD_closeprotected(L, 1, LUA_OK);  /* close all upvalues */
    L->چوٹی.p = L->ڈھیر.p + 1;  /* empty the stack to run finalizers */
    luaC_freeallobjects(L);  /* collect all objects */
    luai_userstateclose(L);
  }
  luaM_freearray(L, G(L)->strt.hash, cast_sizet(G(L)->strt.سائز));
  freestack(L);
  lua_assert(gettotalbytes(g) == حجم(global_State));
  (*g->frealloc)(g->ud, g, حجم(global_State), 0);  /* free main block */
}


LUA_API lua_State *lua_newthread (lua_State *L) {
  global_State *g = G(L);
  GCObject *o;
  lua_State *L1;
  lua_تالا(L);
  luaC_checkGC(L);
  /* create new thread */
  o = luaC_newobjdt(L, LUA_TTHREAD, حجم(LX), offsetof(LX, l));
  L1 = gco2th(o);
  /* anchor it on L stack */
  setthvalue2s(L, L->چوٹی.p, L1);
  api_incr_چوٹی(L);
  preinit_thread(L1, g);
  L1->hookmask = L->hookmask;
  L1->basehookcount = L->basehookcount;
  L1->hook = L->hook;
  resethookcount(L1);
  /* initialize L1 extra space */
  حافظہ_نقل(lua_getextraspace(L1), lua_getextraspace(mainthread(g)),
         LUA_EXTRASPACE);
  luai_userstatethread(L, L1);
  ڈھیر_init(L1, L);  /* init stack */
  lua_unlock(L);
  واپس L1;
}


خالی luaE_freethread (lua_State *L, lua_State *L1) {
  LX *l = fromstate(L1);
  luaF_closeupval(L1, L1->ڈھیر.p);  /* close all upvalues */
  lua_assert(L1->openupval == تہی);
  luai_userstatefree(L, L1);
  freestack(L1);
  luaM_free(L, l);
}


TStatus luaE_resetthread (lua_State *L, TStatus حالت_2) {
  resetCI(L);
  اگر (حالت_2 == LUA_YIELD)
    حالت_2 = LUA_OK;
  حالت_2 = luaD_closeprotected(L, 1, حالت_2);
  اگر (حالت_2 != LUA_OK)  /* errors? */
    luaD_seterrorobj(L, حالت_2, L->ڈھیر.p + 1);
  ورنہ  /* keep in the stack only the fake function-entry for basic ci */
    L->چوٹی.p = L->ڈھیر.p + 1;
  luaD_reallocstack(L, cast_int(L->ci->چوٹی.p - L->ڈھیر.p), 0);
  واپس حالت_2;
}


/*
** Auxiliary function to call luaE_resetthread in protected mode.
** That function closes all tbc variables in protected mode, so the
** only possible "error" is when a tbc variable closes the thread being
** closed and therefore calls 'luaD_throwbaselevel'; in that case,
** control goes directly back to the first call to lua_closethread (not
** to the second), to properly return it.
*/
ساکن خالی resetthread_aux (lua_State *L, خالی *ud) {
  TStatus حالت_2 = *cast(TStatus*, ud);
  حالت_2 = luaE_resetthread(L, حالت_2);
  اگر (حالت_2 != LUA_OK)
    luaD_throw(L, حالت_2);
}


LUA_API صحیح lua_closethread (lua_State *L, lua_State *from) {
  TStatus حالت_2;
  lua_تالا(L);
  L->nCcalls = (from) ? getCcalls(from) : 0;
  حالت_2 = luaD_rawrunprotected(L, resetthread_aux, &L->حالت_2);
  /* stack has error message iff there was an error */
  lua_assert(L->چوٹی.p == L->ڈھیر.p + (حالت_2 == LUA_OK ? 1 : 2));
  اگر (L == from)  /* closing itself? */
    luaD_throwbaselevel(L, حالت_2);
  lua_unlock(L);
  واپس APIstatus(حالت_2);
}


LUA_API lua_State *lua_newstate (lua_Alloc f, خالی *ud, غیرعلامتی بیج) {
  صحیح i;
  lua_State *L;
  global_State *g = cast(global_State*,
                       (*f)(ud, تہی, LUA_TTHREAD, حجم(global_State)));
  اگر (g == تہی) واپس تہی;
  L = &g->mainth.l;
  L->tt = LUA_VTHREAD;
  g->currentwhite = bitmask(WHITE0_ثنائی_ہندسہ);
  L->marked = luaC_white(g);
  preinit_thread(L, g);
  g->allgc = obj2gco(L);  /* by now, only object is the main thread */
  L->اگلا = تہی;
  incnny(L);  /* main thread is always non yieldable */
  g->frealloc = f;
  g->ud = ud;
  g->warnf = تہی;
  g->ud_warn = تہی;
  g->بیج = بیج;
  g->gcstp = GCSTPGC;  /* no GC while building state */
  g->strt.سائز = g->strt.nuse = 0;
  g->strt.hash = تہی;
  setnilvalue(&g->l_registry);
  g->panic = تہی;
  g->gcstate = GCSpause;
  g->gckind = KGC_INC;
  g->gcstopem = 0;
  g->gcemergency = 0;
  g->finobj = g->tobefnz = g->fixedgc = تہی;
  g->firstold1 = g->survival = g->old1 = g->reallyold = تہی;
  g->finobjsur = g->finobjold1 = g->finobjrold = تہی;
  g->sweepgc = تہی;
  g->سرمئی = g->grayagain = تہی;
  g->weak = g->ephemeron = g->allweak = تہی;
  g->twups = تہی;
  g->GCtotalbytes = حجم(global_State);
  g->GCmarked = 0;
  g->GCdebt = 0;
  setivalue(&g->nilvalue, 0);  /* to signal that state is not yet built */
  setgcparam(g, PAUSE, LUAI_GCPAUSE);
  setgcparam(g, STEPMUL, LUAI_GCMUL);
  setgcparam(g, STEPSIZE, LUAI_GCSTEPSIZE);
  setgcparam(g, MINORMUL, LUAI_GENMINORMUL);
  setgcparam(g, MINORMAJOR, LUAI_MINORMAJOR);
  setgcparam(g, MAJORMINOR, LUAI_MAJORMINOR);
  برائے (i=0; i < LUA_NUMTYPES; i++) g->mt[i] = تہی;
  اگر (luaD_rawrunprotected(L, f_luaopen, تہی) != LUA_OK) {
    /* memory allocation error: free partial state */
    close_state(L);
    L = تہی;
  }
  واپس L;
}


LUA_API خالی lua_close (lua_State *L) {
  lua_تالا(L);
  L = mainthread(G(L));  /* only the main thread can be closed */
  close_state(L);
}


خالی luaE_warning (lua_State *L, ثابت حرف *پیغام, صحیح tocont) {
  lua_WarnFunction wf = G(L)->warnf;
  اگر (wf != تہی)
    wf(G(L)->ud_warn, پیغام, tocont);
}


/*
** Generate a warning from an error message
*/
خالی luaE_warnerror (lua_State *L, ثابت حرف *where) {
  TValue *errobj = s2v(L->چوٹی.p - 1);  /* error object */
  ثابت حرف *پیغام = (ttisstring(errobj))
                  ? getstr(tsvalue(errobj))
                  : "error object is not a string";
  /* produce warning "error in %s (%s)" (where, msg) */
  luaE_warning(L, "error in ", 1);
  luaE_warning(L, where, 1);
  luaE_warning(L, " (", 1);
  luaE_warning(L, پیغام, 1);
  luaE_warning(L, ")", 0);
}

