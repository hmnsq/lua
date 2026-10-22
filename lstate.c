/*
** $Id: lstate.c $
** Global State
** See Copyright Notice in lua.h
*/

#تعریف_کړه lstate_c
#تعریف_کړه LUA_CORE

#ضمن "lprefix.h"


#ضمن <stddef.h>
#ضمن <string.h>

#ضمن "lua.h"

#ضمن "lapi.h"
#ضمن "ldebug.h"
#ضمن "ldo.h"
#ضمن "lfunc.h"
#ضمن "lgc.h"
#ضمن "llex.h"
#ضمن "lmem.h"
#ضمن "lstate.h"
#ضمن "lstring.h"
#ضمن "ltable.h"
#ضمن "ltm.h"



#تعریف_کړه fromstate(L)	(cast(LX *, cast(lu_byte *, (L)) - offsetof(LX, l)))


/*
** these macros allow user-specific actions when a thread is
** created/deleted
*/
#إذا !defined(luai_userstateopen)
#تعریف_کړه luai_userstateopen(L)		((فراغ)L)
#د_که_پای

#إذا !defined(luai_userstateclose)
#تعریف_کړه luai_userstateclose(L)		((فراغ)L)
#د_که_پای

#إذا !defined(luai_userstatethread)
#تعریف_کړه luai_userstatethread(L,L1)	((فراغ)L)
#د_که_پای

#إذا !defined(luai_userstatefree)
#تعریف_کړه luai_userstatefree(L,L1)	((فراغ)L)
#د_که_پای


/*
** set GCdebt to a new value keeping the real number of allocated
** objects (GCtotalobjs - GCdebt) invariant and avoiding overflows in
** 'GCtotalobjs'.
*/
فراغ luaE_setdebt (global_State *g, l_mem debt) {
  l_mem tb = gettotalbytes(g);
  lua_assert(tb > 0);
  إذا (debt > MAX_LMEM - tb)
    debt = MAX_LMEM - tb;  /* will make GCtotalbytes == MAX_LMEM */
  g->GCtotalbytes = tb + debt;
  g->GCdebt = debt;
}


CallInfo *luaE_extendCI (lua_State *L, صحيح err) {
  CallInfo *ci;
  ci = luaM_reallocvector(L, عدم, 0, 1, CallInfo);
  إذا (l_unlikely(ci == عدم)) {  /* allocation failed? */
    إذا (err)
      luaM_error(L);  /* raise the error */
    أرجع عدم;  /* else only report it */
  }
  ci->بل = L->ci->بل;
  ci->پخوانی = L->ci;
  L->ci->بل = ci;
  إذا (ci->بل)
    ci->بل->پخوانی = ci;
  ci->u.l.trap = 0;
  L->nci++;
  أرجع ci;
}


/*
** free all CallInfo structures not in use by a thread
*/
ساكن فراغ freeCI (lua_State *L) {
  CallInfo *ci = L->ci;
  CallInfo *بل = ci->بل;
  ci->بل = عدم;
  طالما ((ci = بل) != عدم) {
    بل = ci->بل;
    luaM_free(L, ci);
    L->nci--;
  }
}


/*
** free half of the CallInfo structures not in use by a thread,
** keeping the first one.
*/
فراغ luaE_shrinkCI (lua_State *L) {
  CallInfo *ci = L->ci->بل;  /* first free CallInfo */
  CallInfo *بل;
  إذا (ci == عدم)
    أرجع;  /* no extra elements */
  طالما ((بل = ci->بل) != عدم) {  /* two extra elements? */
    CallInfo *next2 = بل->بل;  /* next's next */
    ci->بل = next2;  /* remove next from the list */
    L->nci--;
    luaM_free(L, بل);  /* free next */
    إذا (next2 == عدم)
      اكسر;  /* no more elements */
    وإلا {
      next2->پخوانی = ci;
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
فراغ luaE_checkcstack (lua_State *L) {
  إذا (getCcalls(L) == LUAI_MAXCCALLS)
    luaG_runerror(L, "C stack overflow");
  وإلا إذا (getCcalls(L) >= (LUAI_MAXCCALLS / 10 * 11))
    luaD_errerr(L);  /* error while handling stack error */
}


LUAI_FUNC فراغ luaE_incCstack (lua_State *L) {
  L->nCcalls++;
  إذا (l_unlikely(getCcalls(L) >= LUAI_MAXCCALLS))
    luaE_checkcstack(L);
}


ساكن فراغ resetCI (lua_State *L) {
  CallInfo *ci = L->ci = &L->بنسټ_ci;
  ci->func.p = L->ډېری.p;
  setnilvalue2s(ci->func.p);  /* 'function' entry for basic 'ci' */
  ci->سر.p = ci->func.p + 1 + LUA_MINSTACK;  /* +1 for 'function' entry */
  ci->u.c.k = عدم;
  ci->callstatus = CIST_C;
  L->حالت = LUA_OK;
  L->errfunc = 0;  /* stack unwind can "throw away" the error function */
}


ساكن فراغ ډېری_init (lua_State *L1, lua_State *L) {
  صحيح i;
  /* initialize stack array */
  L1->ډېری.p = luaM_newvector(L, BASIC_ډېری_اندازه + EXTRA_ډېری, ډېری_ارزښت);
  L1->tbclist.p = L1->ډېری.p;
  لكل (i = 0; i < BASIC_ډېری_اندازه + EXTRA_ډېری; i++)
    setnilvalue2s(L1->ډېری.p + i);  /* erase new stack */
  L1->ډېری_وروستی.p = L1->ډېری.p + BASIC_ډېری_اندازه;
  /* initialize first ci */
  resetCI(L1);
  L1->سر.p = L1->ډېری.p + 1;  /* +1 for 'function' entry */
}


ساكن فراغ freestack (lua_State *L) {
  إذا (L->ډېری.p == عدم)
    أرجع;  /* stack not completely built yet */
  L->ci = &L->بنسټ_ci;  /* free the entire 'ci' list */
  freeCI(L);
  lua_assert(L->nci == 0);
  /* free stack */
  luaM_freearray(L, L->ډېری.p, cast_sizet(stacksize(L) + EXTRA_ډېری));
}


/*
** Create registry table and its predefined values
*/
ساكن فراغ init_registry (lua_State *L, global_State *g) {
  /* create registry */
  TValue aux;
  جدول *registry = luaH_new(L);
  sethvalue(L, &g->l_registry, registry);
  luaH_resize(L, registry, LUA_RIDX_وروستی, 0);
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
ساكن فراغ f_luaopen (lua_State *L, فراغ *ud) {
  global_State *g = G(L);
  ناکارول_شوی(ud);
  ډېری_init(L, L);  /* init stack */
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
ساكن فراغ preinit_thread (lua_State *L, global_State *g) {
  G(L) = g;
  L->ډېری.p = عدم;
  L->ci = عدم;
  L->nci = 0;
  L->twups = L;  /* thread has no upvalues */
  L->nCcalls = 0;
  L->errorJmp = عدم;
  L->hook = عدم;
  L->hookmask = 0;
  L->basehookcount = 0;
  L->allowhook = 1;
  resethookcount(L);
  L->openupval = عدم;
  L->حالت = LUA_OK;
  L->errfunc = 0;
  L->oldpc = 0;
  L->بنسټ_ci.پخوانی = L->بنسټ_ci.بل = عدم;
}


lu_mem luaE_threadsize (lua_State *L) {
  lu_mem sz = cast(lu_mem, حجم(LX))
            + cast_uint(L->nci) * حجم(CallInfo);
  إذا (L->ډېری.p != عدم)
    sz += cast_uint(stacksize(L) + EXTRA_ډېری) * حجم(ډېری_ارزښت);
  أرجع sz;
}


ساكن فراغ close_state (lua_State *L) {
  global_State *g = G(L);
  إذا (!completestate(g))  /* closing a partially built state? */
    luaC_freeallobjects(L);  /* just collect its objects */
  وإلا {  /* closing a fully built state */
    resetCI(L);
    luaD_closeprotected(L, 1, LUA_OK);  /* close all upvalues */
    L->سر.p = L->ډېری.p + 1;  /* empty the stack to run finalizers */
    luaC_freeallobjects(L);  /* collect all objects */
    luai_userstateclose(L);
  }
  luaM_freearray(L, G(L)->strt.hash, cast_sizet(G(L)->strt.اندازه));
  freestack(L);
  lua_assert(gettotalbytes(g) == حجم(global_State));
  (*g->frealloc)(g->ud, g, حجم(global_State), 0);  /* free main block */
}


LUA_API lua_State *lua_newthread (lua_State *L) {
  global_State *g = G(L);
  GCObject *o;
  lua_State *L1;
  lua_قلف(L);
  luaC_checkGC(L);
  /* create new thread */
  o = luaC_newobjdt(L, LUA_TTHREAD, حجم(LX), offsetof(LX, l));
  L1 = gco2th(o);
  /* anchor it on L stack */
  setthvalue2s(L, L->سر.p, L1);
  api_incr_سر(L);
  preinit_thread(L1, g);
  L1->hookmask = L->hookmask;
  L1->basehookcount = L->basehookcount;
  L1->hook = L->hook;
  resethookcount(L1);
  /* initialize L1 extra space */
  حافظه_نقل(lua_getextraspace(L1), lua_getextraspace(mainthread(g)),
         LUA_EXTRASPACE);
  luai_userstatethread(L, L1);
  ډېری_init(L1, L);  /* init stack */
  lua_unlock(L);
  أرجع L1;
}


فراغ luaE_freethread (lua_State *L, lua_State *L1) {
  LX *l = fromstate(L1);
  luaF_closeupval(L1, L1->ډېری.p);  /* close all upvalues */
  lua_assert(L1->openupval == عدم);
  luai_userstatefree(L, L1);
  freestack(L1);
  luaM_free(L, l);
}


TStatus luaE_resetthread (lua_State *L, TStatus حالت) {
  resetCI(L);
  إذا (حالت == LUA_YIELD)
    حالت = LUA_OK;
  حالت = luaD_closeprotected(L, 1, حالت);
  إذا (حالت != LUA_OK)  /* errors? */
    luaD_seterrorobj(L, حالت, L->ډېری.p + 1);
  وإلا  /* keep in the stack only the fake function-entry for basic ci */
    L->سر.p = L->ډېری.p + 1;
  luaD_reallocstack(L, cast_int(L->ci->سر.p - L->ډېری.p), 0);
  أرجع حالت;
}


/*
** Auxiliary function to call luaE_resetthread in protected mode.
** That function closes all tbc variables in protected mode, so the
** only possible "error" is when a tbc variable closes the thread being
** closed and therefore calls 'luaD_throwbaselevel'; in that case,
** control goes directly back to the first call to lua_closethread (not
** to the second), to properly return it.
*/
ساكن فراغ resetthread_aux (lua_State *L, فراغ *ud) {
  TStatus حالت = *cast(TStatus*, ud);
  حالت = luaE_resetthread(L, حالت);
  إذا (حالت != LUA_OK)
    luaD_throw(L, حالت);
}


LUA_API صحيح lua_closethread (lua_State *L, lua_State *from) {
  TStatus حالت;
  lua_قلف(L);
  L->nCcalls = (from) ? getCcalls(from) : 0;
  حالت = luaD_rawrunprotected(L, resetthread_aux, &L->حالت);
  /* stack has error message iff there was an error */
  lua_assert(L->سر.p == L->ډېری.p + (حالت == LUA_OK ? 1 : 2));
  إذا (L == from)  /* closing itself? */
    luaD_throwbaselevel(L, حالت);
  lua_unlock(L);
  أرجع APIstatus(حالت);
}


LUA_API lua_State *lua_newstate (lua_Alloc f, فراغ *ud, بینښه تخم) {
  صحيح i;
  lua_State *L;
  global_State *g = cast(global_State*,
                       (*f)(ud, عدم, LUA_TTHREAD, حجم(global_State)));
  إذا (g == عدم) أرجع عدم;
  L = &g->mainth.l;
  L->tt = LUA_VTHREAD;
  g->currentwhite = bitmask(WHITE0_بټ);
  L->marked = luaC_white(g);
  preinit_thread(L, g);
  g->allgc = obj2gco(L);  /* by now, only object is the main thread */
  L->بل = عدم;
  incnny(L);  /* main thread is always non yieldable */
  g->frealloc = f;
  g->ud = ud;
  g->warnf = عدم;
  g->ud_warn = عدم;
  g->تخم = تخم;
  g->gcstp = GCSTPGC;  /* no GC while building state */
  g->strt.اندازه = g->strt.nuse = 0;
  g->strt.hash = عدم;
  setnilvalue(&g->l_registry);
  g->panic = عدم;
  g->gcstate = GCSpause;
  g->gckind = KGC_INC;
  g->gcstopem = 0;
  g->gcemergency = 0;
  g->finobj = g->tobefnz = g->fixedgc = عدم;
  g->firstold1 = g->survival = g->old1 = g->reallyold = عدم;
  g->finobjsur = g->finobjold1 = g->finobjrold = عدم;
  g->sweepgc = عدم;
  g->خړ = g->grayagain = عدم;
  g->weak = g->ephemeron = g->allweak = عدم;
  g->twups = عدم;
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
  لكل (i=0; i < LUA_NUMTYPES; i++) g->mt[i] = عدم;
  إذا (luaD_rawrunprotected(L, f_luaopen, عدم) != LUA_OK) {
    /* memory allocation error: free partial state */
    close_state(L);
    L = عدم;
  }
  أرجع L;
}


LUA_API فراغ lua_close (lua_State *L) {
  lua_قلف(L);
  L = mainthread(G(L));  /* only the main thread can be closed */
  close_state(L);
}


فراغ luaE_warning (lua_State *L, ثابت حرف *پیغام, صحيح tocont) {
  lua_WarnFunction wf = G(L)->warnf;
  إذا (wf != عدم)
    wf(G(L)->ud_warn, پیغام, tocont);
}


/*
** Generate a warning from an error message
*/
فراغ luaE_warnerror (lua_State *L, ثابت حرف *where) {
  TValue *errobj = s2v(L->سر.p - 1);  /* error object */
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

